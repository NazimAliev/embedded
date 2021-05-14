from rest_framework import serializers
from .models import AnswerText
from .models import AnswerChoice

class PollerSerializer(serializers.Serializer):
	poller_id = serializers.IntegerField()
	poller_title = serializers.CharField(max_length=50)
	poller_text = serializers.CharField(max_length=200)
	start_date = serializers.DateTimeField('start poll')
	end_date = serializers.DateTimeField('end poll')

class QuestionSerializer(serializers.Serializer):
	question_id = serializers.IntegerField()
	question_text = serializers.CharField(max_length=200)
	question_type = serializers.IntegerField()

class AnswerTextSerializer(serializers.Serializer):
	user_id = serializers.IntegerField()
	question_id = serializers.IntegerField()
	answer_text = serializers.CharField(max_length=200)
	def create(self, validated_data):
		return AnswerText.objects.create(**validated_data)

class AnswerChoiceSerializer(serializers.Serializer):
	user_id = serializers.IntegerField()
	choice_id = serializers.IntegerField()
	def create(self, validated_data):
		return AnswerChoice.objects.create(**validated_data)
