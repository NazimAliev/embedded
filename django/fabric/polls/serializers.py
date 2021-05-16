from rest_framework import serializers
from .models import Polls
from .models import Questions 
from .models import Choices 
from .models import Answers 
from .models import AnswerText 
from .models import AnswerOne
from .models import AnswerChoice

# list all polls including title, date and description for each
class PollsSerializer(serializers.Serializer):
	poll_title = serializers.CharField(max_length=50)
	poll_text = serializers.CharField(max_length=200)
	start_date = serializers.DateField()
	end_date = serializers.DateField()

# list selected poll header, list of questions and list of choices for every question
class OnePollSerializer(serializers.ModelSerializer):
	choice_name = serializers.StringRelatedField(many=True)

	class Meta:
		model = Questions 
		fields = ['question','question_text','question_type','choice_name']
		# hide answers fiels
		#exclude = ('checked','answer_text')

class ChoicesSerializer(serializers.ModelSerializer):

	class Meta:
		model = Choices 
		fields = ['choice']

# as above plus answers 
class AnswersSerializer(serializers.ModelSerializer):

	class Meta:
		model = Answers 
		fields = ['answer']

# POST
class AnswerTextSerializer(serializers.Serializer):
	user = serializers.IntegerField()
	question_parent = serializers.IntegerField()
	answer_text = serializers.CharField(max_length=200)

	def create(self, validated_data):
		res = AnswerText.objects.get(answer=validated_data['question_parent'])
		validated_data['question_parent'] = res 
		
		return AnswerText.objects.create(**validated_data)

class AnswerOneSerializer(serializers.Serializer):
	user = serializers.IntegerField()
	choice_parent = serializers.IntegerField()

	def create(self, validated_data):
		res = Choices.objects.get(choice=validated_data['choice_parent'])
		validated_data['choice_parent'] = res 
		return  AnswerOne.objects.create(**validated_data)

class AnswerChoiceSerializer(serializers.Serializer):
	user = serializers.IntegerField()
	choice_parent = serializers.IntegerField()

	def create(self, validated_data):
		res = Choices.objects.get(choice=validated_data['choice_parent'])
		validated_data['choice_parent'] = res 
		return AnswerChoice.objects.create(**validated_data)
